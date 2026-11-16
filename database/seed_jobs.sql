-- ============================================================
-- SmartHire Database Seed Jobs
-- Standard initial jobs for recruitment database
-- ============================================================

INSERT OR IGNORE INTO jobs (job_id, title, company, location, employment_type, education_requirement, minimum_experience, description)
VALUES 
('J001', 'Software Engineer', 'Infosys Ltd.', 'Pune', 'Full Time', 'B.Tech/BE Computer Science', 2, 'Core backend and systems development with emphasis on algorithms and software architecture.'),
('J002', 'C++ Software Developer', 'Persistent Systems', 'Pune', 'Full Time', 'B.Tech/BE Computer Engineering', 1, 'High-performance C++ software engineering focusing on low-latency systems and object-oriented design.'),
('J003', 'Web Developer', 'XYZ Software', 'Remote', 'Full Time', 'Any Graduate', 0, 'Modern full-stack web engineering, building responsive frontends and secure REST API backends.'),
('J004', 'Java Backend Developer', 'Wipro Technologies', 'Bangalore', 'Full Time', 'B.Tech/MCA', 2, 'Enterprise Java services, microservices architecture, and scalable cloud applications.'),
('J005', 'Full Stack Developer', 'TCS Digital', 'Mumbai', 'Full Time', 'B.Tech Computer Science/IT', 1, 'End-to-end full stack development across modern web frameworks and enterprise database architectures.');
